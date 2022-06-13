#include "main.h"

// TODO Download method for Ubidots salinity & weather readings.
// TODO Data transformation to calculate closure severity index
// TODO Data transformation to calculate heat severity index

int main(void){

	curl_global_init(CURL_GLOBAL_ALL);

    //Ubidots_Devices_TypeDef devices = {0};
    //Ubidots_ListDevices(&devices);
    //Ubidots_DevicesToCSV(&devices);

   // IBM_AuthHandle_TypeDef ibm_auth_handle = {0};
   // if(IBM_HandleAuth(&ibm_auth_handle) != 0){
   //     return 1;
   // }

   // IBM_TimeseriesReq_TypeDef ibm_req = {
   //         .layer_id = 49097, // 16700 (alt_flag = 0) or 49097 (alt_flag = 1)
   //         .latitude = -35.69701049568654,
   //         .longitude = 150.1546566614602,
   //         .start = 1654824507,
   //         .end = 1655306906
   // };

   // IBM_TimeseriesDataset_TypeDef dataset;
   // IBM_GetTimeseries(&ibm_auth_handle, &ibm_req, &dataset, 1);

   // IBM_TimeseriesToCSV(&ibm_req, &dataset);

    //WillyWeather_GetToken("WW_TOKEN");

    //const char* search_location = "Batemans";
    //WW_Location_TypeDef location_info = {0};
    //WillyWeather_GetLocationByName(search_location, &location_info);

    //WW_TideDataset_TypeDef tides = {0};
    //WillyWeather_GetTides(location_info.id, "2022-01-01", 6, &tides);

    //WillyWeather_TidesToCSV(&location_info, &tides);

    FA_HarvestAreaStatus_TypeDef hs_status;
    FA_GetHarvestAreaStatus(FA_HA_CLYDE_MOONLIGHT, &hs_status);


    curl_global_cleanup();

    return 0;
}