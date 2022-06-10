#include "utils.h"

/// Helper function to handle a CURL request response which normally gets
/// written to stdout.
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                                  void *userp);

/**
 * Callback function to save HTTP response data to a character array.
 *
 * CURL defaults to printing to stdout. This function is added to CURL
 * in the setup options (curl_easy_setupopt()). Memory is allocated and then
 * filled with the response string. This character array is then used to create
 * cJSON objects and other items.
 *
 * @code
 *      MemoryStruct_TypeDef chunk;
 *      chunk.memory = malloc(1);
 *      chunk.size = 0;
 *      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
 *      curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
 *      free(chunk.memory) // Don't forget to free allocation
 * @endcode
 *
 * @see https://curl.se/libcurl/c/getinmemory.html
 *
 * @param contents The items to add.
 * @param size The size of items to add.
 * @param nmemb The number of members (data size).
 * @param userp The struct to populate with data.
 * @return The size of received data.
 */
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
		void *userp){
	size_t realsize = size * nmemb;

	MemoryStruct_TypeDef *mem = (MemoryStruct_TypeDef *)userp;

	char *ptr = realloc(mem->memory, mem->size + realsize + 1);

	if(ptr == NULL){
		fprintf(stderr, "[Error]: Not enough memory to hold HTTP response "
                        "data.\n");
		return 0;
	}

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

/**
 * Basic HTTP request using CURL.
 *
 * Most initialisation is done outside of this function. This function just
 * handles the HTTP request and puts the data inside a provided cJSON
 * object. Note this function has a memory leak on a Mac M1 ->
 * (curl_easy_perform()) has 13 leaks, totalling 496 bytes per call.
 *
 * @code
 *      const char *URL = "https://www.example.com";
 *      // Create some headers (more may be needed depending on application)
 *
 *      struct curl_slist *headers = NULL;
 *      headers = curl_slist_append(headers, "Content-Type: application/json");
 *
 *      // Create a cJSON object to hold the response
 *      cJSON *response = NULL;
 *      CURLcode result = HttpRequest(&response, URL, headers, 0, NULL);
 *
 *      // Free memory
 *      curl_slist_free_all(headers);
 *      cJSON_Delete(response);
 * @endcode
 *
 * @param response The cJSON response to populate with JSON.
 * @param URL The request URL.
 * @param headers Headers to include in request.
 * @param post Interger flag to request POST request (set to 1).
 * @param body POST request body.
 * @return Status code representing the response status.
 */
CURLcode HttpRequest(cJSON **response, const char *URL,
                struct curl_slist *headers, int8_t post, const char* body){

    CURL *curl = curl_easy_init();
    if(!curl){
        fprintf(stderr, "Curl not found. Exiting.\n");
        return CURLE_FUNCTION_NOT_FOUND;
    }

    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "dpi-agent"); // Needed sometimes
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L); // Fail on 400 - 500 errors

    // Post request requested
    if(post == 1){
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    }

    MemoryStruct_TypeDef chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    CURLcode result = curl_easy_perform(curl);

    if(result != CURLE_OK){
        fprintf(stderr, "curl request failed: %s\n",
                curl_easy_strerror(result));
    } else {
        *response = cJSON_Parse(chunk.memory);
    }

    free(chunk.memory);
    curl_easy_cleanup(curl);

    return result;
}

/**
 * Helper function to create directories with error handling.
 *
 * @code
 * MakeDirectory("my_directory");
 * MakeDirectory("my_directory/my_sub_directory");
 * @endcode
 *
 * @param directory Directory name you want to create.
 * @return Integer representing error status. 0 = OK ... 1 = ERROR
 */
int8_t MakeDirectory(const char* directory){
    if(mkdir(directory, S_IRWXU | S_IRWXG | S_IRWXO) == -1){
        // If the directory already exists it not an error
        if(errno != EEXIST){
            fprintf(stderr, "[Error]: Unable to create directory: %s -> ",
                    directory);
            switch(errno){
                case EACCES:
                    fprintf(stderr, "Permission denied.\n");
                    return 1;
                case ENOENT:
                    fprintf(stderr, "Path does not exist.\n");
                    return 1;
                default:
                    fprintf(stderr, "Unknown error.\n");
                    return 1;
            }
        }
    } else {
        fprintf(stdout, "[Info]: Created directory: %s\n", directory);
    }
    return 0;
}

/**
 * Helper function to handle writing timeseries data to a .csv file.
 *
 * Writes timeseries data to a filename, exiting when the data is
 * un-initialised '\0' or the maxiumum buffer size is reached. This function
 * also converts UNIX time into local (human readable) time in ISO 8601
 * format.
 *
 * @code
 * // Build filename (directory requried)
 * char filename[100];
 * sprintf(filename, "%s/low.csv", directory);
 * WriteTimeseriesToFile(filename,
 *                       dataset->low_tide_timestamps, // UNIX timestamps
 *                       dataset->low_tide_values, // Values
 *                       WW_FORECAST_RESPONSE_BUF); // Max number of values
 * @endcode
 *
 * @param filename File to write to.
 * @param dates Dates (x values).
 * @param values Data values (y values).
 * @param max_n_values Max buffer size.
 */
void WriteTimeseriesToFile(const char* filename, time_t* dates, double* values,
                           uint16_t max_n_values){

    FILE *file = fopen(filename, "w+");
    fprintf(file, "UNIX;Date;Tide(m)\n");

    uint16_t index = 0;
    while(dates[index] != '\0' && index < max_n_values){
        char date[25] = {0};
        struct tm *date_tm = localtime(&dates[index]);
        strftime(date, 25, "%Y-%m-%dT%H:%M:%S%z", date_tm);
        fprintf(file, "%ld;%s;%lf\n", dates[index], date, values[index]);
        index++;
    }

    fclose(file);
}