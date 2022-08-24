#include "transform.h"

static void T_PreparedStatements(PGconn* psql_conn){
    if(db_prepared){
        return;
    }

    // Get historical and future forecast information
    const char* fcst_stmt_name = "SelectPrecipForecast";
    PGresult* fcst_info = PQdescribePrepared(psql_conn, fcst_stmt_name);
    if(PQresultStatus(fcst_info) != PGRES_COMMAND_OK){
        const char* fcst_stmt = "SELECT ts, forecast_precipitation "
                                "FROM weather "
                                "WHERE program_id = $1::int "
                                "ORDER BY ts ASC "
                                "LIMIT 20;";
        PGresult* fcst_prep = PQprepare(psql_conn, fcst_stmt_name, fcst_stmt,
                                        1, NULL);
        if(PQresultStatus(fcst_prep) != PGRES_COMMAND_OK){
            log_error("PostgreSQL prepare error: %s\n",
                      PQerrorMessage(psql_conn));
        }
        PQclear(fcst_prep);
    }
    PQclear(fcst_info);

    // Prepare SQL statement for inserting (updating) intermediate log
    // tranformed values
    const char* log_stmt_name = "InsertLogPrecip";
    PGresult* log_info = PQdescribePrepared(psql_conn, log_stmt_name);
    if(PQresultStatus(log_info) != PGRES_COMMAND_OK){
        const char* log_stmt = "UPDATE weather "
                               "SET log_precip = $1::float "
                               "WHERE program_id = $2::int "
                               "AND ts = $3::timestamptz;";
        PGresult* log_prep = PQprepare(psql_conn, log_stmt_name, log_stmt,
                                       1, NULL);
        if(PQresultStatus(log_prep) != PGRES_COMMAND_OK){
            log_error("PostgreSQL prepare error: %s\n",
                      PQerrorMessage(psql_conn));
        }
        PQclear(log_prep);
    }
    PQclear(log_info);

    // Prepare SQL statement for inserting (updating) Z-Score values
    const char* zs_stmt_name = "InsertZScorePrecip";
    PGresult* zs_info = PQdescribePrepared(psql_conn, zs_stmt_name);
    if(PQresultStatus(zs_info) != PGRES_COMMAND_OK){
        const char* zs_stmt = "UPDATE weather "
                              "SET forecast_zscore_precip = $1::float "
                              "WHERE program_id = $2::int "
                              "AND ts = $3::timestamptz;";
        PGresult* zs_prep = PQprepare(psql_conn, zs_stmt_name, zs_stmt,
                                      1, NULL);
        if(PQresultStatus(zs_prep) != PGRES_COMMAND_OK){
            log_error("PostgreSQL prepare error: %s\n",
                      PQerrorMessage(psql_conn));
        }
        PQclear(zs_prep);
    }
    PQclear(zs_info);

    const char* stats_stmt_name = "SelectPrecipStats";
    PGresult* stats_info = PQdescribePrepared(psql_conn, stats_stmt_name);
    if(PQresultStatus(stats_info) != PGRES_COMMAND_OK){
        log_debug("Adding prepared SQL statement: %s\n", stats_stmt_name);
        const char* stmt = "SELECT "
                           "MIN(log_precip), "
                           "MAX(log_precip), "
                           "AVG(log_precip), "
                           "STDDEV(log_precip) "
                           "FROM weather "
                           "WHERE program_id = $1::int;";
        PGresult* stats_prep = PQprepare(psql_conn, stats_stmt_name, stmt, 1,
                                         NULL);
        if(PQresultStatus(stats_prep) != PGRES_COMMAND_OK){
            log_error("PostgreSQL prepare error: %s\n",
                      PQerrorMessage(psql_conn));
        }
        PQclear(stats_prep);
    }
    PQclear(stats_info);

    db_prepared = true;
}

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

    T_PreparedStatements(psql_conn);

    // Add required paramters to prepared statement
    // Looking to query by program_id
    const char* params[1];
    int id = 5; // TODO make this dynamic
    char buf[10];
    snprintf(buf, sizeof(buf), "%d", id);
    params[0] = buf;

    // Insert preicpitation parameters buffers
    const char* weather_params[3];
    char zs_buf[15]; // Z-Score value
    // program_id from previous `buf`
    char ts_buf[30];

    // First itteration just parses the log precipitation value for each row
    // and puts these data into their own column `log_precip`
    Utils_PrepareStatement(psql_conn, "SelectPrecipForecast",
                           "SELECT ts, forecast_precipitation "
                           "FROM weather "
                           "WHERE program_id = $1::int "
                           "ORDER BY ts ASC "
                           "LIMIT 20;");
    PGresult* fcst_res = PQexecPrepared(psql_conn, "SelectPrecipForecast", 1,
                                         params, NULL, NULL, 0);
    char* ptr;
    double fcst_precip = 0;
    if(PQresultStatus(fcst_res) == PGRES_TUPLES_OK) {
        int num_fields = PQnfields(fcst_res);
        for (int i = 0; i < PQntuples(fcst_res); i++) {
            memset(ts_buf, 0, sizeof(ts_buf));
            for (int j = 0; j < num_fields; j++) {
                switch(j){
                    case 0:
                        // Timestamp
                        strncpy(ts_buf, PQgetvalue(fcst_res, i, j),
                                sizeof(ts_buf));
                        break;
                    case 1:
                        // Forecast precipitation
                        fcst_precip = strtod(PQgetvalue(fcst_res, i, j), &ptr);
                        break;
                    default:
                        log_error("Unexpected value in query response.\n");
                        break;
                }
            }
            /* LOG10 TRANSFORMATION START */
            double log_precip = 0;
            if(fcst_precip > 0){
                log_precip = log10(fcst_precip);
            }
            /* LOG10 TRANSFORMATION END */

            log_debug("%s:\t%f\t%f\n", ts_buf, fcst_precip, log_precip);

            snprintf(zs_buf, sizeof(zs_buf), "%f", log_precip);
            weather_params[0] = zs_buf; // Z-Score buffer
            weather_params[1] = buf; // program_id buffer
            weather_params[2] = ts_buf; // Timestamp buffer

            PGresult* fcst_insert = PQexecPrepared(psql_conn, log_stmt_name, 3,
                                         weather_params, NULL, NULL, 1);
            if(PQresultStatus(fcst_insert) != PGRES_COMMAND_OK){
                log_error("PostgreSQL transformation insert error: %s\n",
                          PQerrorMessage(psql_conn));
            }
            PQclear(fcst_insert);
        }
    }
    PQclear(fcst_res);

    PGresult* stats_res = PQexecPrepared(psql_conn, stats_stmt_name, 1,
                                         params, NULL, NULL, 0);
    double min = 0, max = 0, avg = 0, stdev = 0;
    if(PQresultStatus(stats_res) == PGRES_TUPLES_OK){
        int num_fields = PQnfields(stats_res);
        for(int i = 0; i < PQntuples(stats_res); i++){
            for(int j = 0; j < num_fields; j++){
                double value = strtod(PQgetvalue(stats_res, i, j), &ptr);
                switch(j){
                    case 0:
                        min = value;
                        break;
                    case 1:
                        max = value;
                        break;
                    case 2:
                        avg = value;
                        break;
                    case 3:
                        stdev = value;
                        break;
                    default:
                        log_error("Unexpected value in query response.\n");
                        break;
                }
            }
        }
    }
    log_info("Min: %f\tMax: %f\tAvg: %f\tStdev: %f\n", min, max, avg, stdev);
    PQclear(stats_res);

    // Calculate z-score
    //double z_score = (log_precip - avg) / stdev;

    //snprintf(zs_buf, sizeof(zs_buf), "%f", z_score);
    //weather_params[0] = zs_buf; // Z-Score buffer
    //weather_params[1] = buf; // program_id buffer
    //weather_params[2] = ts_buf; // Timestamp buffer

    //fcst_insert = PQexecPrepared(psql_conn, zs_stmt_name, 3,
    //                             weather_params, NULL, NULL, 1);
    //if(PQresultStatus(fcst_insert) != PGRES_COMMAND_OK){
    //    log_error("PostgreSQL transformation insert error: %s\n",
    //              PQerrorMessage(psql_conn));
    //}
}
