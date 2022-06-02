#include "../include/main.h"

int main(void){

    // Initialise CURL (required)
	curl_global_init(CURL_GLOBAL_ALL);

    // Load ubidots token
	const char *ibm_token = getenv("IBM_TOKEN");
	if(ibm_token == NULL) {
		printf("IBM token not found. Exiting.\n");
		return EXIT_FAILURE;
	}

    // Required to cleanup curl
    curl_global_cleanup();

    return 0;
}