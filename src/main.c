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

    HarvestAreaStatus_TypeDef ha_status;
    FoodAuth_GetHarvestAreaStatus("Moonlight", &ha_status);

    // Required to cleanup curl
    curl_global_cleanup();

    return 0;
}