#include "main.h"

// TODO Download method for IBM precipitation data.
// TODO Download method for Ubidots salinity & weather readings.
// TODO Download method for Willy Weather forecast (tides etc.)
// TODO Data transformation to calculate closure severity index
// TODO Data transformation to calculate heat severity index

int main(void){

    // Initialise CURL (required)
	curl_global_init(CURL_GLOBAL_ALL);

    // Load ubidots token
	const char *ibm_token = getenv("IBM_TOKEN");
	if(ibm_token == NULL) {
		printf("IBM token not found. Exiting.\n");
		return EXIT_FAILURE;
	}

    char* access_token = malloc(IBM_ACCESS_TOKEN_SIZE); // Access token to populate
    char* refresh_token = malloc(IBM_REFRESH_TOKEN_SIZE); // Refresh token to populate
    IBM_Authenticate(ibm_token, refresh_token, access_token);

    TimeseriesReq_TypeDef ts = {
            .layer_id = 49097, // 16700 (alt_flag = 0) or 49097 (alt_flag = 1)
            .latitude = -35.69701049568654,
            .longitude = 150.1546566614602,
            .start = 1654005600,
            .end = 1654783200
    };

    TimeseriesDataset_TypeDef dataset;
    IBM_GetTimeseries(access_token, &ts, &dataset, 1);

    free(access_token);
    free(refresh_token);

    // Required to cleanup curl
    curl_global_cleanup();

    return 0;
}