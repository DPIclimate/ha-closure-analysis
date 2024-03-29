#include "transform.h"

/**
 * @brief Helper function for nomalising data.
 *
 * @param value Raw input value.
 * @param min Raw maxiumum value.
 * @param max Raw miniumum value.
 * @return Normalised input value (between 0 and 1).
 */
static inline float T_Normalise(const float value,
                                const float min,
                                const float max){
    return (value - min) / (max - min);
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
    Utils_PrepareStatement(psql_conn, forecast_stmt_name,
                           "INSERT INTO weather (last_updated, latitude, "
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
                           "forecast_precipitation = $11::float;", 11);
    const char* forecast_paramValues[11];

    const char* bom_select ="SELECT ts AT TIME ZONE 'AEST', precipitation "
                            "FROM weather_bom WHERE location_id = '%s' "
                            "ORDER BY (ts) DESC;";
    char bom_query[200];

    // Prepare weather table insert statement (using BOM (historical) data)
    const char* historical_stmt_name = "InsertBOMHistorical";
    Utils_PrepareStatement(psql_conn, historical_stmt_name,
                           "INSERT INTO weather (last_updated, "
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
                           "observed_precipitation = $11::float;", 11);
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

/**
 * @breif Main entry point for calculating the risk of a harvest area closure.
 *
 * Gets a list of the harvest areas. Loops through each of these locations
 * to calcualte a windowed moving average of precipitation and then normalises
 * this window to provide a value between 0 and 1.
 *
 * @param psql_conn PostgreSQL connection.
 */
void T_FloodPrediction(PGconn* psql_conn){

    const char* stmt_name = "SelectProgramIds";
    Utils_PrepareStatement(psql_conn, stmt_name,
                           "SELECT fa_program_id FROM harvest_lookup;", 0);

    char name_buf[100];
    PGresult* program_res = PQexec(psql_conn, "SELECT fa_program_name, "
                                              "fa_program_id FROM "
                                              "harvest_lookup;");
    if(PQresultStatus(program_res) == PGRES_TUPLES_OK){
        int n_fields = PQnfields(program_res);
        char* ptr;
        for(int i = 0; i < PQntuples(program_res); i++){
            int program_id = -1;
            for(int j = 0; j < n_fields; j++){
                switch(j){
                    case 0:
                        // Program name
                        strncpy(name_buf, PQgetvalue(program_res, i, j),
                                sizeof(name_buf));
                        break;
                    case 1:
                        // Program ID
                        program_id = (int)strtol(PQgetvalue(program_res, i, j),
                                            &ptr, 10);
                        break;
                    default:
                        log_error("Unexpected value in query response.\n");
                        break;
                }

            }
            if(program_id == -1){
                log_fatal("PostgreSQL error selecting program_id: %s\n",
                          PQerrorMessage(psql_conn));
                return;
            }

            log_debug("Transforming (%d of %d):\t%s (ID: %d)\n", i + 1,
                      PQntuples(program_res), name_buf, program_id);

            T_WindowDataset(psql_conn, program_id);
            T_NormaliseWindowedPrecipitation(psql_conn, program_id);
        }

    }

    PQclear(program_res);
}

/**
 * @brief Window dataset into sum of next 8-days and previous 5-days.
 *
 * For each `program_id` calcualte the sum of the next 8-days precipitation and
 * the previous 5-days preciptiation. This acts as a moving window across all
 * available data.
 *
 * @note The outlook and hindlook can be adjusted by altering the `w_start` and
 * `w_end` variables in this function.
 *
 * @param psql_conn PostgreSQL connection.
 * @param program_id Food authority program ID.
 */
void T_WindowDataset(PGconn* psql_conn, const int program_id){

    const char* params[1];
    char buf[10];
    snprintf(buf, sizeof(buf), "%d", program_id);
    params[0] = buf;

    const char* fcst_stmt_name = "SelectPrecipForecast";
    Utils_PrepareStatement(psql_conn, fcst_stmt_name,
                           "SELECT ts, forecast_precipitation "
                           "FROM weather "
                           "WHERE program_id = $1::int "
                           "ORDER BY ts ASC;", 1);

    const char* window_stmt_name = "InsertWindowedSum";
    Utils_PrepareStatement(psql_conn, window_stmt_name,
                           "UPDATE weather SET sum_precip = $1::float "
                           "WHERE program_id = $2::int "
                           "AND ts = $3::timestamptz;", 3);

    PGresult* fcst_res = PQexecPrepared(psql_conn, fcst_stmt_name, 1,
                                            params, NULL, NULL, 0);
    const int ts_buf_size = 30;
    char* ptr;
    if(PQresultStatus(fcst_res) == PGRES_TUPLES_OK) {
        int num_fields = PQnfields(fcst_res);
        char** timestamps = malloc((unsigned long) PQntuples(fcst_res) *
                sizeof(char*));
        float* values = malloc((unsigned long)PQntuples(fcst_res) *
                sizeof(float));
        for (int i = 0; i < PQntuples(fcst_res); i++) {
            for (int j = 0; j < num_fields; j++) {
                switch(j){
                    case 0:
                        // Timestamp
                        timestamps[i] = malloc(ts_buf_size + 1);
                        strncpy(timestamps[i], PQgetvalue(fcst_res, i, j),
                                ts_buf_size);
                        break;
                    case 1:
                        // Precipitation
                        values[i] = strtof(PQgetvalue(fcst_res, i, j), &ptr);
                        break;
                    default:
                        log_error("Unexpected value in query response.\n");
                        break;
                }
            }
        }

        /* Summed Data windowing */
        int w_start = 5; // Days prior to sum
        int w_end = 8; // Forecasted days (max)
        // Total window size = w_start + w_end

        // Insert parameters and buffers
        const char* w_params[3];
        char val_buf[10];

        for(int i = w_start; i < PQntuples(fcst_res) - w_end; i++){
            float sum = 0;
            for(int x = i - w_start; x < (i + w_end); x++){
                sum += values[x];
            }

            snprintf(val_buf, sizeof(val_buf), "%f", sum);
            w_params[0] = val_buf;
            w_params[1] = buf;
            w_params[2] = timestamps[i];

            PGresult* w_res = PQexecPrepared(psql_conn, window_stmt_name, 3,
                                             w_params, NULL, NULL, 0);
            if(PQresultStatus(w_res) != PGRES_COMMAND_OK){
                log_error("PostgreSQL windowed data insert error: %s\n",
                          PQerrorMessage(psql_conn));
            }

            PQclear(w_res);
            free(timestamps[i]);
        }

        free(timestamps);
        free(values);
    }

    PQclear(fcst_res);

}

/**
 * @breif Normalise precipiation data between 0 and 1.
 *
 * After a summed window average has been completed by `T_WindowDataset` the
 * data can then be normalised. This normalisation is program specific i.e.
 * each normalisations max and min are based on that program only and not the
 * max and min of all programs in NSW.
 *
 * @param psql_conn PostgreSQL connection.
 */
void T_NormaliseWindowedPrecipitation(PGconn* psql_conn, const int program_id){

    const char* params[1];
    char buf[10];
    snprintf(buf, sizeof(buf), "%d", program_id);
    params[0] = buf;

    const char* stats_stmt_name = "SelectWindowStats";
    Utils_PrepareStatement(psql_conn, stats_stmt_name,
                           "SELECT "
                           "MIN(sum_precip), "
                           "MAX(sum_precip) "
                           "FROM weather WHERE program_id = $1::int;", 1);

    PGresult* stats_res = PQexecPrepared(psql_conn, stats_stmt_name, 1, params,
                                         NULL, NULL, 0);
    char* ptr;
    float min = 0, max = 0;
    if(PQresultStatus(stats_res) == PGRES_TUPLES_OK){
        min = strtof(PQgetvalue(stats_res, 0, 0), &ptr);
        max = strtof(PQgetvalue(stats_res, 0, 1), &ptr);
    } else {
        log_fatal("PostgreSQL stats select error: %s\n",
                  PQerrorMessage(psql_conn));
        return;
    }
    PQclear(stats_res);

    const char* norm_stmt_name = "InsertNormalisedPrecip";
    Utils_PrepareStatement(psql_conn, norm_stmt_name,
                           "UPDATE weather "
                           "SET normalised_precip = "
                           "CASE WHEN sum_precip IS NOT NULL "
                           "THEN $1::float END "
                           "WHERE program_id = $2::int "
                           "AND ts = $3::timestamptz;", 3);

    const char* fcst_stmt_name = "SelectWindowedForecast";
    Utils_PrepareStatement(psql_conn, fcst_stmt_name,
                           "SELECT ts, sum_precip "
                           "FROM weather "
                           "WHERE program_id = $1::int "
                           "ORDER BY ts ASC;", 1);

    PGresult* fcst_res = PQexecPrepared(psql_conn, fcst_stmt_name, 1,
                                        params, NULL, NULL, 0);
    const int ts_buf_size = 30;
    if(PQresultStatus(fcst_res) == PGRES_TUPLES_OK) {
        int num_fields = PQnfields(fcst_res);
        char **timestamps = malloc((unsigned long) PQntuples(fcst_res) *
                                   sizeof(char *));
        float *values = malloc((unsigned long) PQntuples(fcst_res) *
                               sizeof(float));
        for (int i = 0; i < PQntuples(fcst_res); i++) {
            for (int j = 0; j < num_fields; j++) {
                switch (j) {
                    case 0:
                        // Timestamp
                        timestamps[i] = malloc(ts_buf_size + 1);
                        strncpy(timestamps[i], PQgetvalue(fcst_res, i, j),
                                ts_buf_size);
                        break;
                    case 1:
                        // Precipitation
                        values[i] = strtof(PQgetvalue(fcst_res, i, j), &ptr);
                        break;
                    default:
                        log_error("Unexpected value in query response.\n");
                        break;
                }
            }
        }

        /* Normalise windowed data between 0 and 1 */
        const char* norm_params[3];
        char val_buf[10];

        for(int i = 0; i < PQntuples(fcst_res); i++){
            float norm_val = T_Normalise(values[i], min, max);
            snprintf(val_buf, sizeof(val_buf), "%f", norm_val);
            norm_params[0] = val_buf;
            norm_params[1] = buf;
            norm_params[2] = timestamps[i];

            PGresult* norm_res = PQexecPrepared(psql_conn, norm_stmt_name, 3,
                                             norm_params, NULL, NULL, 0);
            if(PQresultStatus(norm_res) != PGRES_COMMAND_OK){
                log_error("PostgreSQL windowed data insert error: %s\n",
                          PQerrorMessage(psql_conn));
            }

            PQclear(norm_res);
            free(timestamps[i]);
        }
        free(timestamps);
        free(values);
    }
    PQclear(fcst_res);
}
