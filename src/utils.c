#include "utils.h"

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
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                           void *userp) {
    size_t realsize = size * nmemb;

    Utils_ReqData_TypeDef *mem = (Utils_ReqData_TypeDef *) userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);

    if (ptr == NULL) {
        log_error("Not enough memory to hold HTTP response data.\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
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
int8_t MakeDirectory(const char *directory) {
    if (mkdir(directory, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
        // If the directory already exists it not an error
        if (errno != EEXIST) {
            log_error("Unable to create directory: %s\n",
                      directory);
            switch (errno) {
                case EACCES:
                    log_error("Permission denied.\n");
                    return 1;
                case ENOENT:
                    log_error("Path does not exist.\n");
                    return 1;
                default:
                    log_error("Unknown error.\n");
                    return 1;
            }
        }
    } else {
        log_info("Created directory: %s\n", directory);
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
void WriteTimeseriesToFile(const char *filename, time_t *dates, double *values,
                           int16_t max_n_values) {

    log_info("Writing timeseries dataset to %s\n", filename);

    FILE *file = fopen(filename, "w+");
    fprintf(file, "UNIX;Date;Data\n");

    uint16_t index = 0;
    while (dates[index] != '\0' && index < max_n_values) {
        char date[25] = {0};
        struct tm *date_tm = localtime(&dates[index]);
        strftime(date, 25, "%Y-%m-%dT%H:%M:%S%z", date_tm);
        fprintf(file, "%ld;%s;%lf\n", dates[index], date, values[index]);
        index++;
    }

    fclose(file);
}

/**
 * HTML minify function modified from cJSON library.
 *
 * The main cJSON_Minify() function gets rid of all white space and '/'
 * characters in a string. This however, causes sentances to appear without
 * the nessessary spaces between words. This modified function keeps only
 * one space between each character (if more than one space occurs) and
 * maintains the spaces between words in a sentance. Additionally '/'
 * characters are maintained as they are used as datetime delimiters.
 *
 * @see https://github.com/DaveGamble/cJSON/blob/master/cJSON.c#L2838
 *
 * @param json The string to minify (parse).
 */
void cJSON_Minify_Mod(char *json) {
    char *into = json;

    if (json == NULL) {
        return;
    }

    char prev_char;
    while (json[0] != '\0') {
        switch (json[0]) {
            case ' ':
                if (prev_char != ' ' && prev_char != '\n') {
                    into[0] = json[0];
                    into++;
                }
                prev_char = json[0];
                json++;
                break;
            case '\t':
            case '\r':
            case '\n':
                prev_char = json[0];
                json++;
                break;
            default:
                prev_char = json[0];
                into[0] = json[0];
                json++;
                into++;
        }
    }

    /* and null-terminate. */
    *into = '\0';
}

/**
 * Uses haversine formula to calculate the distance in km between two
 * points on earth.
 *
 * Used to calculate the distance between an POI and the closest Bureau of
 * Meterology station.
 *
 * @param latitude Latitude of point of interest.
 * @param longitude Longitude of point of interest.
 * @param station_latitude Latitude of station.
 * @param station_longitude Longitude of station.
 * @return Distance between two points on earth in km.
 */
double Utils_PointsDistance(double latitude,
                            double longitude,
                            double station_latitude,
                            double station_longitude) {
    double p = M_PI / 180;
    return 12742.0 * asin(sqrt(
            (0.5 - (cos((latitude - station_latitude) * p) / 2.0) +
             (cos(station_latitude * p) * cos(latitude * p)) *
             ((1 - cos((longitude - station_longitude) * p)) / 2.0))));
}
