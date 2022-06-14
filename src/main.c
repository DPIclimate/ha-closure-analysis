#include "main.h"

// TODO Tide and precipitation line up datasets
// TODO Data transformation to calculate closure severity index

// TODO Download method for Ubidots salinity & weather readings.
// TODO Data transformation to calculate heat severity index

int main(void){

	curl_global_init(CURL_GLOBAL_ALL);

    const char* start_time = "2022-05-01";
    struct tm start_dt = {0};
    strptime(start_time, "%Y-%02m-%02d", &start_dt);
    const time_t unix_st = mktime(&start_dt);

    const char* end_time = "2022-06-01";
    struct tm end_dt = {0};
    strptime(end_time, "%Y-%02m-%02d", &end_dt);
    const time_t unix_et = mktime(&end_dt);

    // Convert to days between dates (for Willy Weather)
    time_t day_diff = (((unix_et - unix_st) / 60) / 60) / 24;

    log_debug("Start time: %s (%ld), End time: %s (%ld), Number of days: %ld"
              "\n", start_time, unix_st, end_time, unix_et, day_diff);

    IBM_AuthHandle_TypeDef ibm_auth_handle = {0};
    if(IBM_HandleAuth(&ibm_auth_handle) != 0){
        return 1;
    }

    IBM_TimeseriesReq_TypeDef ibm_req = {
            .layer_id = 49097, // 16700 (alt_flag = 0) or 49097 (alt_flag = 1)
            .latitude = -35.69701049568654,
            .longitude = 150.1546566614602,
            .start = unix_st,
            .end = unix_et
    };

    IBM_TimeseriesDataset_TypeDef dataset;
    IBM_GetTimeseries(&ibm_auth_handle, &ibm_req, &dataset, 1);
    IBM_TimeseriesToCSV(&ibm_req, &dataset);

    const char* search_location = "Batemans";
    WW_Location_TypeDef location_info = {0};
    WillyWeather_GetLocationByName(search_location, &location_info);

    WW_TideDataset_TypeDef tides = {0};
    WillyWeather_GetTides(location_info.id, start_time, day_diff, &tides);
    WillyWeather_TidesToCSV(&location_info, &tides);

    assert(day_diff == tides.n_days);

    //FA_HarvestAreaStatus_TypeDef hs_status;
    //FA_GetHarvestAreaStatus(FA_HA_CLYDE_MOONLIGHT, &hs_status);

    curl_global_cleanup();

    return 0;
}