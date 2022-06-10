#include "main.h"

// TODO Download method for IBM precipitation data.
// TODO Download method for Ubidots salinity & weather readings.
// TODO Data transformation to calculate closure severity index
// TODO Data transformation to calculate heat severity index

int main(void){

	curl_global_init(CURL_GLOBAL_ALL);

    IBM_AuthHandle_TypeDef auth_handle = {0};
    if(IBM_HandleAuth(&auth_handle) != 0){
        fprintf(stderr, "[Error]: Unable to authenticate with IBM EIS.\n");
        return 1;
    }

    if(IBM_HandleAuth(&auth_handle) != 0){
        fprintf(stderr, "[Error]: Unable to authenticate with IBM EIS.\n");
        return 1;
    }


    //WillyWeather_GetToken("WW_TOKEN");

    //const char* search_location = "Batemans";
    //WW_Location_TypeDef location_info = {0};
    //WillyWeather_GetLocationByName(search_location, &location_info);

    //WW_TideDataset_TypeDef tides = {0};
    //WillyWeather_GetTides(location_info.id, "2022-01-01", 6, &tides);

    //WillyWeather_TidesToCSV(&location_info, &tides);

    //FA_HarvestAreaStatus_TypeDef hs_status;
    //FA_GetHarvestAreaStatus(FA_HA_CLYDE_MOONLIGHT, &hs_status);

    //const char *ibm_token = getenv("IBM_TOKEN");
    //if(ibm_token == NULL) {
    //    printf("Token not found. Exiting.\n");
    //    return EXIT_FAILURE;
    //}

    //char* access_token = malloc(IBM_ACCESS_TOKEN_SIZE); // Access token to populate
    //char* refresh_token = malloc(IBM_REFRESH_TOKEN_SIZE); // Refresh token to populate
    //IBM_Authenticate(ibm_token, refresh_token, access_token);

    //IBM_TimeseriesReq_TypeDef ts = {
    //        .layer_id = 49097, // 16700 (alt_flag = 0) or 49097 (alt_flag = 1)
    //        .latitude = -35.69701049568654,
    //        .longitude = 150.1546566614602,
    //        .start = 1654005600,
    //        .end = 1654783200
    //};

    //IBM_TimeseriesDataset_TypeDef dataset;
    //IBM_GetTimeseries(access_token, &ts, &dataset, 1);

    //free(access_token);
    //free(refresh_token);

    curl_global_cleanup();

    return 0;
}