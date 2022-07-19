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
                                   "longitude, ts, program_name, bom_location_id, "
                                   "data_type, precipitation, "
                                   "forecast_precipitation) "
                                   "VALUES (NOW(), $1::float, $2::float, "
                                   "$3::timestamptz, $4::text, $5::text, "
                                   "$6::text, $7::float, $8::float) "
                                   "ON CONFLICT (ts, program_name) DO UPDATE "
                                   "SET last_updated = NOW(), "
                                   "data_type = 'forecast', "
                                   "precipitation = $9::float, "
                                   "forecast_precipitation = $10::float;";
        PGresult* forecast_prep = PQprepare(psql_conn, forecast_stmt_name,
                                            forecast_stmt, 1, NULL);
        if(PQresultStatus(forecast_prep) != PGRES_COMMAND_OK){
            log_warn("PostgreSQL prepare error: %s\n",
                     PQerrorMessage(psql_conn));
        }
        PQclear(forecast_prep);
    }
    PQclear(forecast_info);

    const char* forecast_paramValues[10];

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
                                      "bom_location_id, data_type, "
                                      "precipitation, observed_precipitation) "
                                      "VALUES (NOW(), $1::float, $2::float, "
                                      "$3::timestamptz, $4::text, $5::text, "
                                      "$6::text, $7::float, $8::float) "
                                      "ON CONFLICT (ts, program_name) DO "
                                      "UPDATE SET last_updated = NOW(), "
                                      "data_type = 'observed', "
                                      "precipitation = $9::float, "
                                      "observed_precipitation = $10::float;";
        PGresult* historical_prep = PQprepare(psql_conn, historical_stmt_name,
                                              historical_stmt, 1, NULL);
        if(PQresultStatus(historical_prep) != PGRES_COMMAND_OK){
            log_warn("PostgreSQL prepare error: %s\n",
                     PQerrorMessage(psql_conn));
        }
        PQclear(historical_prep);
    }
    PQclear(historical_info);

    const char* historical_paramValues[10];

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
                forecast_paramValues[4] = loc.bom_location_id;
                forecast_paramValues[5] = "forecast";
                snprintf(precip_buf, sizeof(precip_buf), "%f",
                         ibm_precipitation);
                forecast_paramValues[6] = precip_buf;
                forecast_paramValues[7] = precip_buf;
                forecast_paramValues[8] = precip_buf;
                forecast_paramValues[9] = precip_buf;

                // Insert weather forecast data from IBM
                PGresult* ibm_insert_query =
                        PQexecPrepared(psql_conn, forecast_stmt_name, 10,
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
                            bom_precipitation = strtof(PQgetvalue(bom_res, i, j),
                                                       &ptr);
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
                historical_paramValues[4] = loc.bom_location_id;
                historical_paramValues[5] = "observed";
                snprintf(precip_buf, sizeof(precip_buf), "%f",
                         bom_precipitation);
                historical_paramValues[6] = precip_buf;
                historical_paramValues[7] = precip_buf;
                historical_paramValues[8] = precip_buf;
                historical_paramValues[9] = precip_buf;

                PGresult* bom_insert_query =
                        PQexecPrepared(psql_conn, historical_stmt_name, 10,
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

    // Get a unique list of the latest harvest area statuses
    const char* fa_query = "SELECT DISTINCT ON (id) program_name, id, "
                           "classification, status, time_processed, "
                           "status_reason FROM harvest_area "
                           "ORDER BY id, time_processed DESC;";

    PGresult* fa_res = PQexec(psql_conn, fa_query);

    if(PQresultStatus(fa_res) == PGRES_TUPLES_OK){
        int num_fields = PQnfields(fa_res);
        for(int i = 0; i < PQntuples(fa_res); i++){

            // Get basic current harvest area status information
            T_Outlook_Typedef outlook;
            char *ptr;
            for(int j = 0; j < num_fields; j++){
                switch(j){
                    case 0: // Program name
                        strncpy(outlook.program_name, PQgetvalue(fa_res, i, j),
                                sizeof(outlook.program_name));
                        break;
                    case 1: // ID
                        outlook.ha_id = (int32_t)strtol(PQgetvalue(fa_res, i, j),
                                                        &ptr, 10);
                        break;
                    case 2: // Classification
                        strncpy(outlook.classification, PQgetvalue(fa_res, i, j),
                                sizeof(outlook.classification));
                        break;
                    case 3: // Status
                        strncpy(outlook.status, PQgetvalue(fa_res, i, j),
                                sizeof(outlook.status));
                        break;
                    case 4: // Time processed
                        strncpy(outlook.time_processed, PQgetvalue(fa_res, i, j),
                                sizeof(outlook.time_processed));
                        break;
                    case 5: // Reason for current status
                        strncpy(outlook.status_reason, PQgetvalue(fa_res, i, j),
                                sizeof(outlook.status_reason));
                        break;
                    default:
                        log_error("Unknown value recived when parsing harvest "
                                  "area status information.\n");
                        return;
                }
            }

            // Get current observed weather and forecast information for a
            // particular harvest area
            char weather_query[300];
            snprintf(weather_query, sizeof(weather_query),
                     "SELECT ts, data_type, precipitation FROM weather WHERE "
                     "program_name = '%s' ORDER BY ts DESC LIMIT %d;",
                     outlook.program_name, T_DATA_QUERY_LENGTH);

            PGresult* weather_res = PQexec(psql_conn, weather_query);
            T_OutlookDataset_TypeDef weather_dataset = {0};
            weather_dataset.count = 0;
            if(PQresultStatus(weather_res) == PGRES_TUPLES_OK){
                int w_num_fields = PQnfields(weather_res);
                for(int x = PQntuples(weather_res)-1; x >= 0; x--) {
                    T_OutlookData_TypeDef value = {0};
                    char* p;
                    for (int j = 0; j < w_num_fields; j++) {
                        switch(j){
                            case 0:
                                strncpy(value.time_processed,
                                        PQgetvalue(weather_res, x, j),
                                        sizeof(value.time_processed));
                                break;
                            case 1:
                                strncpy(value.data_type,
                                        PQgetvalue(weather_res, x, j),
                                        sizeof(value.data_type));
                                break;
                            case 2:
                                value.precipitaiton = (double)strtof(
                                        PQgetvalue(weather_res, x, j), &p);
                                break;
                            default:
                                log_error("Unknown weather information "
                                          "queried.\n");
                                return;
                        }
                    }
                    weather_dataset.data[weather_dataset.count] = value;
                    weather_dataset.count++;
                }
            }

            // Setup 7-day total moving window and calculate if daily
            // precipitation is going to exceed a daily threshold
            double sum_precip = 0;
            bool daily_total_closure = false;
            for(int y = 0; y < weather_dataset.count; y++){
                double daily_precip = weather_dataset.data[y].precipitaiton;
                if(y < T_DATA_WINDOW_SIZE){
                    sum_precip += daily_precip;
                }

                if(daily_precip > 30.0 && !daily_total_closure){
                    T_CloseHarvestArea(&outlook,
                                       "Rainfall is predicted to exceed "
                                       "30 mm in a single day.",
                                       weather_dataset.data[y].time_processed);
                    daily_total_closure = true;
                }
            }

            // Moving window to calcualte 7-day total rainfall
            double seven_day_window_sum = sum_precip;
            for(int p = T_DATA_WINDOW_SIZE; p < weather_dataset.count; p++){
                seven_day_window_sum +=
                        weather_dataset.data[p].precipitaiton -
                        weather_dataset.data[p -
                        T_DATA_WINDOW_SIZE].precipitaiton;
                if(sum_precip < seven_day_window_sum){
                    sum_precip = seven_day_window_sum;
                }
            }

            if(sum_precip > 100.0){
                T_CloseHarvestArea(&outlook,
                                   "Rainfall is predicted to exceed "
                                   "100 mm in a 7-day period", "timestamp");
            }
        }
    }
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
