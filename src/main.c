#include "main.h"

// TODO Data transformation to calculate closure severity index

// TODO FTP BOM Data? May not be available
// TODO To get site details: http://www.bom.gov.au/jsp/ncc/cdio/weatherData/av?p_stn_num=69023&p_display_type=availableYears&p_nccObsCode=136
// TODO Could use: http://www.bom.gov.au/jsp/ncc/cdio/weatherData/av?p_display_type=dailyZippedDataFile&p_stn_num=69023&p_c=-952881996&p_nccObsCode=136&p_startYear=2020

// TODO Download method for Ubidots salinity & weather readings.
// TODO Data transformation to calculate heat severity index

int main(void){

	curl_global_init(CURL_GLOBAL_ALL);

    IBM_TimeseriesDataset_TypeDef ibm_dataset = IBM_TimeseriesFromCSV(
            "datasets/ibm/timeseries/precipitation.csv");

    WW_TideDataset_TypeDef tide_dataset = WW_TidesFromCSV(
            "datasets/tides/Batemans-Bay/daily-max.csv");

    assert(ibm_dataset.count == tide_dataset.n_days);

    int8_t res = SI_CalculateFloodRisk(
            tide_dataset.daily_max_tide_values,
            ibm_dataset.values,
            ibm_dataset.timestamps,
            ibm_dataset.count);

    switch(res){
        case SI_MIN_DAYS_ERROR:
            log_error("Error calculating severity index.\n");
            break;
        case SI_ALLOCATION_EXCEEDED:
            log_error("Error number of days exceeds allocated memory.\n");
            break;
        case SI_OK:
            log_info("Severity index was sucessfully calculated.\n");
            break;
        default:
            log_error("Unknown error occured when calculated SI.\n");
            break;
    }

  //  const char* start_time = "2022-05-01";
  //  struct tm start_dt = {0};
  //  strptime(start_time, "%Y-%02m-%02d", &start_dt);
  //  const time_t unix_st = mktime(&start_dt);

  //  const char* end_time = "2022-06-01";
  //  struct tm end_dt = {0};
  //  strptime(end_time, "%Y-%02m-%02d", &end_dt);
  //  const time_t unix_et = mktime(&end_dt);

  //  // Convert to days between dates (for Willy Weather)
  //  time_t day_diff = (((unix_et - unix_st) / 60) / 60) / 24;

  //  log_debug("Start time: %s (%ld), End time: %s (%ld), Number of days: %ld"
  //            "\n", start_time, unix_st, end_time, unix_et, day_diff);

  //  const char* search_location = "Batemans";
  //  WW_Location_TypeDef location_info = {0};
  //  WillyWeather_GetLocationByName(search_location, &location_info);

  //  WW_TideDataset_TypeDef tides = {0};
  //  WillyWeather_GetTides(location_info.id, start_time, day_diff, &tides);
  //  WillyWeather_TidesToCSV(&location_info, &tides);

  //  IBM_AuthHandle_TypeDef ibm_auth_handle = {0};
  //  if(IBM_HandleAuth(&ibm_auth_handle) != 0){
  //      return 1;
  //  }

  //  IBM_TimeseriesReq_TypeDef ibm_req = {
  //          .layer_id = 49097, // 16700 (alt_flag = 0) or 49097 (alt_flag = 1)
  //          .latitude = location_info.latitude,
  //          .longitude = location_info.longitude,
  //          .start = unix_st,
  //          .end = unix_et
  //  };

  //  IBM_TimeseriesDataset_TypeDef dataset;
  //  IBM_GetTimeseries(&ibm_auth_handle, &ibm_req, &dataset, 1);
  //  IBM_TimeseriesToCSV(&ibm_req, &dataset);

  //  assert(day_diff == tides.n_days);

    //FA_HarvestAreaStatus_TypeDef hs_status;
    //FA_GetHarvestAreaStatus(FA_HA_CLYDE_MOONLIGHT, &hs_status);

    curl_global_cleanup();

    return 0;
}